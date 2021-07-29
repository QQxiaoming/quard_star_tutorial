(:*******************************************************:)
(: Test: K-NormalizeUnicodeFunc-9                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A test whose essence is: `normalize-unicode("f oo", "") eq "f oo"`. :)
(:*******************************************************:)
normalize-unicode("f   oo",  "") eq "f   oo"