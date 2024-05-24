#ifndef _KERN_DEV_KVM_H_
#define _KERN_DEV_KVM_H_

#ifdef _KERN_

int detect_kvm(void);
uint64_t kvm_get_tsc_hz(void);

#endif /* _KERN_ */
#endif /* !_KERN_DEV_KVM_H_ */