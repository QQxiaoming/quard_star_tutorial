(:*******************************************************:)
(: Test: K-SeqAVGFunc-14                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: A test whose essence is: `string(avg((3, 3, xs:double("NaN")))) eq "NaN"`. :)
(:*******************************************************:)
string(avg((3, 3, xs:double("NaN")))) eq "NaN"