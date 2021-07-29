(:*******************************************************:)
(: Test: K-NormalizeSpaceFunc-7                          :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A test whose essence is: `normalize-space("f o o ") eq "f o o"`. :)
(:*******************************************************:)
normalize-space("f	  o  	o ") eq "f o o"