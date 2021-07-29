(:*******************************************************:)
(: Test: K2-ForExprWith-1                                :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: For declarations does not cause type promotions to apply. :)
(:*******************************************************:)
for $i as xs:string in xs:untypedAtomic("input")
return $i