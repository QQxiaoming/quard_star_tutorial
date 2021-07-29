(:*******************************************************:)
(: Test: K-NormalizeSpaceFunc-8                          :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A test whose essence is: `normalize-space(" 143 1239 fhjkls ") eq "143 1239 fhjkls"`. :)
(:*******************************************************:)
normalize-space("	143 
			1239 fhjkls	") eq "143 1239 fhjkls"