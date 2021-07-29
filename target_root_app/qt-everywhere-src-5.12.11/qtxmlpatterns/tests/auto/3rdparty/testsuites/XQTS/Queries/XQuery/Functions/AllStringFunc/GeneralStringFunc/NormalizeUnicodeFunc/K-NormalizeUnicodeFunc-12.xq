(:*******************************************************:)
(: Test: K-NormalizeUnicodeFunc-12                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A test whose essence is: `normalize-unicode("f oo", "NFC") eq "f oo"`. :)
(:*******************************************************:)
normalize-unicode("f		oo", "NFC")
			eq "f		oo"