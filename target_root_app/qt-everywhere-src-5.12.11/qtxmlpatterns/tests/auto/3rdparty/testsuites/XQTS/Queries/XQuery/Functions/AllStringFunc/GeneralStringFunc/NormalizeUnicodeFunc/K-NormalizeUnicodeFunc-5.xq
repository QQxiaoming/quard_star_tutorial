(:*******************************************************:)
(: Test: K-NormalizeUnicodeFunc-5                        :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A test whose essence is: `normalize-unicode("foo", "NFC") eq "foo"`. :)
(:*******************************************************:)
normalize-unicode("foo", "NFC") eq "foo"