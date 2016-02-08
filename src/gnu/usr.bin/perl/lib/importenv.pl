;# $RCSfile: importenv.pl,v $$Revision: 1.2 $$Date: 1997/11/30 07:57:01 $

;# This file, when interpreted, pulls the environment into normal variables.
;# Usage:
;#	require 'importenv.pl';
;# or
;#	#include <importenv.pl>

local($tmp,$key) = '';

foreach $key (keys(%ENV)) {
    $tmp .= "\$$key = \$ENV{'$key'};" if $key =~ /^[A-Za-z]\w*$/;
}
eval $tmp;

1;
