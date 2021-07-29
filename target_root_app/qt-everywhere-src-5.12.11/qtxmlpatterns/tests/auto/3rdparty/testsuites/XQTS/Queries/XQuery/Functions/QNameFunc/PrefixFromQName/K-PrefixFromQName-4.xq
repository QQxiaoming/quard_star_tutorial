(:*******************************************************:)
(: Test: K-PrefixFromQName-4                             :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A test whose essence is: `prefix-from-QName( QName("example.com/", "pre:lname")) eq "pre"`. :)
(:*******************************************************:)
prefix-from-QName(
			QName("example.com/", "pre:lname")) eq "pre"