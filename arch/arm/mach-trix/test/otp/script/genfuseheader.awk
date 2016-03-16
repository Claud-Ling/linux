#!/usr/bin/awk -f
#
# Author: Tony He
# Date: 2015-10-30
#
# translate fuse_data_map.inc to autofuse.h in form of
#
# OTP_FUSE_ENTRY(name)
# OTP_FUSE_FIELD(name, fname)
# OTP_FUSE_DATA(name, len)
#

NR == 1 {
	print ("/* ==> Do not modify this file!!  It is created automatically");
	printf("   from %s\n   using the genfuseheader.awk script.\n", FILENAME);
	"date \"+%x %T\"" | getline d;
	printf("   (%s) <== */\n\n", d);
}

/^[ \t]*otp_fuse_entry/ {
	op = $1;
	gsub(/otp_fuse_entry\(/,"",op);
	pos++;
	entry[pos] = op;
	printf("OTP_FUSE_ENTRY(%s)\n", op);

	# otp_fuse_entry(name, ofs, otp_fuse_field(bit, nbits, fname),
	if (match($0, /otp_fuse_field\([0-9]+,[ \t]+[0-9]+,[ \t_A-Z0-9]+\)/) > 0) {
		field = substr($0, RSTART, RLENGTH);
		if (split(field, fnodes, ",") > 2) {
			op = fnodes[3];
			gsub(/\)/,"",op);
			gsub(/^[ \t]+/,"",op);
			printf("OTP_FUSE_FIELD(%s, %s)\n", entry[pos], op);
		}
	}
}

/^[ \t]*otp_fuse_field/ {
	if (pos >= 0) {
		op = $3;
		gsub(/\)/,"",op);
		gsub(/^[ \t]+/,"",op);
		printf("OTP_FUSE_FIELD(%s, %s)\n", entry[pos], op);
	} else {
		err++;
		lineno[err] = NR;
		error[err] = "expect entry before field";
	}
}

/^[ \t]*otp_fuse_generic/ {
	op1 = $1;
	op2 = $3;
	gsub(/[ \t]*otp_fuse_generic\(/,"",op1);
	gsub(/^[ \t]+/,"",op2);
	gsub(/\)/,"",op2);
	printf("OTP_FUSE_DATA(%s, %s)\n", op1, op2);
}

BEGIN {
	pos = -1;
	err = -1;
	FS = ",";
}

END {
	for (i = 0; i <= err; i++) {
		printf("%s:%s: error: %s\n", FILENAME, lineno[i], error[i]);
	}
}
