(:*******************************************************:)
(: Test: K-SeqAVGFunc-23                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: A test whose essence is: `string(avg((xs:float("NaN"), 1, 2, 3))) eq "NaN"`. :)
(:*******************************************************:)
string(avg((xs:float("NaN"), 1, 2, 3))) eq "NaN"