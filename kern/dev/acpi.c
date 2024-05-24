#include <lib/debug.h>
#include <lib/types.h>

#include "acpi.h"

static uint8_t sum(uint8_t *addr, int len)
{
    int i, sum;

    sum = 0;
    for (i = 0; i < len; i++) {
        sum += addr[i];
    }

    return sum;
}

static acpi_rsdp_t *acpi_probe_rsdp_aux(uint8_t *addr, int length)
{
    uint8_t *e, *p;

    e = addr + length;
    for (p = addr; p < e; p += 16) {
        if (*(uint32_t *) p == ACPI_RSDP_SIG1 &&
            *(uint32_t *) (p + 4) == ACPI_RSDP_SIG2 &&
            sum(p, sizeof(acpi_rsdp_t)) == 0) {
            return (acpi_rsdp_t *) p;
        }
    }

    return NULL;
}

acpi_rsdp_t *acpi_probe_rsdp(void)
{
    uint8_t *bda;
    uint32_t p;
    acpi_rsdp_t *rsdp;

    bda = (uint8_t *) 0x400;
    if ((p = ((bda[0x0F] << 8) | bda[0x0E]) << 4)) {
        if ((rsdp = acpi_probe_rsdp_aux((uint8_t *) p, 1024)))
            return rsdp;
    }

    return acpi_probe_rsdp_aux((uint8_t *) 0xE0000, 0x1FFFF);
}

acpi_rsdt_t *acpi_probe_rsdt(acpi_rsdp_t *rsdp)
{
    KERN_ASSERT(rsdp != NULL);

    acpi_rsdt_t *rsdt = (acpi_rsdt_t *) (rsdp->rsdt_addr);
    if (rsdt == NULL)
        return NULL;
    if (rsdt->sig == ACPI_RSDT_SIG && sum((uint8_t *) rsdt, rsdt->length) == 0) {
        return rsdt;
    }

    return NULL;
}

acpi_sdt_hdr_t *acpi_probe_rsdt_ent(acpi_rsdt_t *rsdt, const uint32_t sig)
{
    KERN_ASSERT(rsdt != NULL);

    uint8_t *p = (uint8_t *) &rsdt->ent[0];
    uint8_t *e = (uint8_t *) rsdt + rsdt->length;

    int i;
    for (i = 0; p < e; i++) {
        acpi_sdt_hdr_t *hdr = (acpi_sdt_hdr_t *) (rsdt->ent[i]);
        if (hdr->sig == sig && sum((uint8_t *) hdr, hdr->length) == 0) {
            return hdr;
        }
        p = (uint8_t *) &rsdt->ent[i + 1];
    }

    return NULL;
}

acpi_xsdt_t *acpi_probe_xsdt(acpi_rsdp_t *rsdp)
{
    KERN_ASSERT(rsdp != NULL);

    acpi_xsdt_t *xsdt = (acpi_xsdt_t *) (uintptr_t) rsdp->xsdt_addr;
    if (xsdt == NULL)
        return NULL;
    if (xsdt->sig == ACPI_XSDT_SIG && sum((uint8_t *) xsdt, xsdt->length) == 0) {
        return xsdt;
    }

    return NULL;
}

acpi_sdt_hdr_t *acpi_probe_xsdt_ent(acpi_xsdt_t *xsdt, const uint32_t sig)
{
    KERN_ASSERT(xsdt != NULL);

    uint8_t *p = (uint8_t *) &xsdt->ent[0];
    uint8_t *e = (uint8_t *) xsdt + xsdt->length;

    int i;
    for (i = 0; p < e; i++) {
        acpi_sdt_hdr_t *hdr = (acpi_sdt_hdr_t *) (uintptr_t) xsdt->ent[i];
        if (hdr->sig == sig && sum((uint8_t *) hdr, hdr->length) == 0) {
            return hdr;
        }
        p = (uint8_t *) &xsdt->ent[i + 1];
    }

    return NULL;
}
