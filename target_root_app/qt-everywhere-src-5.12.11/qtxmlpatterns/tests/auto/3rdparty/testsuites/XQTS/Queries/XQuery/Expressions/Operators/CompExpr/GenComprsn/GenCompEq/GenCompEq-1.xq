(:*******************************************************:)
(: Test: GenCompEq-1                                     :)
(: Written by: Sorin Nasoi                               :)
(: Date: 2009-05-01+02:00                                :)
(: Purpose: General comparison where both types are      :)
(: instances of xs:untypedAtomic.                        :)
(:*******************************************************:)

xs:untypedAtomic("a string") = xs:untypedAtomic("a stringDIFF")