#include "defs.h"
#include <linux/kexec.h>

static const struct xlat kexec_arch_values[] = {
	XLAT(KEXEC_ARCH_DEFAULT),
	XLAT(KEXEC_ARCH_386),
	XLAT(KEXEC_ARCH_X86_64),
	XLAT(KEXEC_ARCH_PPC),
	XLAT(KEXEC_ARCH_PPC64),
	XLAT(KEXEC_ARCH_IA_64),
	XLAT(KEXEC_ARCH_ARM),
	XLAT(KEXEC_ARCH_S390),
	XLAT(KEXEC_ARCH_SH),
	XLAT(KEXEC_ARCH_MIPS_LE),
	XLAT(KEXEC_ARCH_MIPS),
	XLAT_END
};

static const struct xlat kexec_flags[] = {
	XLAT(KEXEC_ON_CRASH),
	XLAT(KEXEC_PRESERVE_CONTEXT),
	XLAT_END
};

static void
print_kexec_segments(struct tcb *tcp, unsigned long addr, unsigned long len)
{
#if SUPPORTED_PERSONALITIES > 1
	union {
		struct { u_int32_t buf, bufsz, mem, memsz; } seg32;
		struct { u_int64_t buf, bufsz, mem, memsz; } seg64;
	} seg;
# define sizeof_seg \
	(current_wordsize == 4 ? sizeof(seg.seg32) : sizeof(seg.seg64))
# define seg_buf \
	(current_wordsize == 4 ? (uint64_t) seg.seg32.buf : seg.seg64.buf)
# define seg_bufsz \
	(current_wordsize == 4 ? (uint64_t) seg.seg32.bufsz : seg.seg64.bufsz)
# define seg_mem \
	(current_wordsize == 4 ? (uint64_t) seg.seg32.mem : seg.seg64.mem)
# define seg_memsz \
	(current_wordsize == 4 ? (uint64_t) seg.seg32.memsz : seg.seg64.memsz)
#else
	struct kexec_segment seg;
# define sizeof_seg sizeof(seg)
# define seg_buf seg.seg_buf
# define seg_bufsz seg.seg_bufsz
# define seg_mem seg.seg_mem
# define seg_memsz seg.seg_memsz
#endif
	unsigned int i, failed;

	if (!len) {
		tprints("[]");
		return;
	}

	if (len > KEXEC_SEGMENT_MAX) {
		tprintf("%#lx", addr);
		return;
	}

	failed = 0;
	tprints("[");
	for (i = 0; i < len; ++i) {
		if (i)
			tprints(", ");
		if (umoven(tcp, addr + i * sizeof_seg, sizeof_seg,
			   (char *) &seg) < 0) {
			tprints("?");
			failed = 1;
			break;
		}
		tprintf("{%#lx, %lu, %#lx, %lu}",
			(long) seg_buf, (unsigned long) seg_bufsz,
			(long) seg_mem, (unsigned long) seg_memsz);
	}
	tprints("]");
	if (failed)
		tprintf(" %#lx", addr);
}

int
sys_kexec_load(struct tcb *tcp)
{
	unsigned long n;

	if (exiting(tcp))
		return 0;

	/* entry, nr_segments */
	tprintf("%#lx, %lu, ", tcp->u_arg[0], tcp->u_arg[1]);

	/* segments */
	print_kexec_segments(tcp, tcp->u_arg[2], tcp->u_arg[1]);
	tprints(", ");

	/* flags */
	n = tcp->u_arg[3];
	printxval(kexec_arch_values, n & KEXEC_ARCH_MASK, "KEXEC_ARCH_???");
	n &= ~KEXEC_ARCH_MASK;
	if (n) {
		tprints("|");
		printflags(kexec_flags, n, "KEXEC_???");
	}

	return 0;
}