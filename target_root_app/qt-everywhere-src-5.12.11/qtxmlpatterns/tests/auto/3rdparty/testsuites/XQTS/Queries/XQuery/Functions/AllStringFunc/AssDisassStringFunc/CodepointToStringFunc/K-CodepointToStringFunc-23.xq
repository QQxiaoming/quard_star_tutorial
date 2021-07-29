(:*******************************************************:)
(: Test: K-CodepointToStringFunc-23                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A test whose essence is: `codepoints-to-string(65533) eq "&#xFFFD;"`. :)
(:*******************************************************:)
codepoints-to-string(65533) eq "&#xFFFD;"