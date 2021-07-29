(:*******************************************************:)
(: Test: K-NormalizeUnicodeFunc-7                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A test whose essence is: `normalize-unicode("foo", "NFKD") eq "foo"`. :)
(:*******************************************************:)
normalize-unicode("foo", "NFKD") eq "foo"