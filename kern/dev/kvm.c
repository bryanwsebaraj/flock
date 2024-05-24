#include <lib/string.h>
#include <lib/types.h>
#include <lib/gcc.h>
#include <lib/x86.h>
#include <lib/debug.h>

#define CPUID_KVM_SIGNATURE			(0x40000000u)

#define CPUID_KVM_FEATURES			(0x40000001u)
/* eax = an OR'ed group of (1 << flag), where each flags is: */
#define KVM_FEATURE_CLOCKSOURCE		0	/* kvmclock available at msrs 0x11 and 0x12 */
#define KVM_FEATURE_CLOCKSOURCE2	3	/* kvmclock available at msrs 0x4b564d00 and 0x4b564d01 */
#define KVM_FEATURE_STEAL_TIME		5	/* steal time can be enabled by writing to msr 0x4b564d03 */

/* edx = an OR'ed group of (1 << flag), where each flags is: */
#define KVM_HINTS_REALTIME			0

/* MSR for system time */
#define MSR_KVM_SYSTEM_TIME_NEW		(0x4b564d01u)
#define MSR_KVM_SYSTEM_TIME			(0x12u)

struct pvclock_vcpu_time_info_t {
	uint32_t	version;
	uint32_t	pad0;
	uint64_t	tsc_timestamp;
	uint64_t	system_time;
	uint32_t	tsc_to_system_mul;
	int8_t		tsc_shift;
	uint8_t		flags;
	uint8_t		pad[2];
} gcc_aligned(4)  gcc_packed; /* 4-byte aligned 32 bytes */

struct pvclock_vcpu_time_info_t pvclock;

gcc_inline int32_t
cpu_has (int flag)
{
	uint32_t eax, ebx, ecx, edx;

	eax = (flag & 0x100) ? 7 : (flag & 0x20) ? 0x80000001 : 1;
	ecx = 0;

	__asm __volatile("cpuid"
			: "+a" (eax), "=b" (ebx), "=d" (edx), "+c" (ecx));

	return ((flag & 0x100 ? ebx : (flag & 0x80) ? ecx : edx) >> (flag & 31)) & 1;
}

#define CPUID_FEATURE_HYPERVISOR	(1<<31) /* Running on a hypervisor */

int detect_kvm(void)
{
	uint32_t eax;

	if (cpu_has (CPUID_FEATURE_HYPERVISOR))
	{
		uint32_t hyper_vendor_id[3];

		cpuid (CPUID_KVM_SIGNATURE, &eax, &hyper_vendor_id[0],
				&hyper_vendor_id[1], &hyper_vendor_id[2]);
		if (!strncmp ("KVMKVMKVM", (const char *) hyper_vendor_id, 9))
		{
			return 1;
		}
	}
	return 0;
}

int
kvm_has_feature(uint32_t feature)
{
	uint32_t eax, ebx, ecx, edx;
	eax = 0; edx = 0;
	cpuid(CPUID_KVM_FEATURES, &eax, &ebx, &ecx, &edx);

	return ((eax & feature) != 0 ? 1 : 0);
}

int
kvm_enable_feature(uint32_t feature)
{
	uint32_t eax, ebx, ecx, edx;
	eax = 1 << feature; edx = 0;
	cpuid(CPUID_KVM_FEATURES, &eax, &ebx, &ecx, &edx);

	return (ebx == 0 ? 1 : 0);
}

uint64_t
kvm_get_tsc_hz(void)
{
	uint64_t tsc_hz = 0llu;
	uint32_t msr_sys_time;

	if (kvm_has_feature(KVM_FEATURE_CLOCKSOURCE2))
	{
		msr_sys_time = MSR_KVM_SYSTEM_TIME_NEW;
	}
	else if (kvm_has_feature(KVM_FEATURE_CLOCKSOURCE))
	{
		msr_sys_time = MSR_KVM_SYSTEM_TIME;
	}
	else
	{
		return (0llu);
	}

	/* bit0 == 1 means enable, kvm will update this memory periodically */
	wrmsr(msr_sys_time, (uint64_t) ((uint32_t) &pvclock) | 0x1llu);

	tsc_hz = (uint64_t) pvclock.tsc_to_system_mul;

	/* disable update */
	wrmsr(msr_sys_time, (uint64_t) ((uint32_t) &pvclock));

	return tsc_hz;
}
