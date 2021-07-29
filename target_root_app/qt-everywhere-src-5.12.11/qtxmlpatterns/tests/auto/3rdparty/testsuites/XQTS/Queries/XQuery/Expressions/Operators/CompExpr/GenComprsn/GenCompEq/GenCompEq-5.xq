(:*******************************************************:)
(: Test: GenCompEq-5                                     :)
(: Written by: Sorin Nasoi                               :)
(: Date: 2009-05-01+02:00                                :)
(: Purpose: General comparison where one of the types is :)
(: xs:untypedAtomic and the other xs:yearMonthDuration   :)
(:*******************************************************:)

    xs:untypedAtomic("P1999Y10M") =
xs:yearMonthDuration("P1999Y10M")