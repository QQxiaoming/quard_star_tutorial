(:*******************************************************:)
(: Test: K-CodepointToStringFunc-26                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: A test whose essence is: `codepoints-to-string(65536) eq "&#x10000;"`. :)
(:*******************************************************:)
codepoints-to-string(65536) eq "&#x10000;"