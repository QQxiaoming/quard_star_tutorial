(:*******************************************************:)
(: Test: K-SeqAVGFunc-41                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: A test whose essence is: `string(avg((xs:float('INF'), xs:float('-INF')))) eq "NaN"`. :)
(:*******************************************************:)
string(avg((xs:float('INF'), xs:float('-INF')))) eq "NaN"