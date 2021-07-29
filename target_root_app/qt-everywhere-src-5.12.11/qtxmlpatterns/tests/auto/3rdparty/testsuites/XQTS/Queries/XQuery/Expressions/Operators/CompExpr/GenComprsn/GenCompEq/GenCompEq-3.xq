(:*******************************************************:)
(: Test: GenCompEq-3                                     :)
(: Written by: Sorin Nasoi                               :)
(: Date: 2009-05-01+02:00                                :)
(: Purpose: General comparison where one of the types is :)
(: xs:untypedAtomic and the other xs:dayTimeDuration     :)
(:*******************************************************:)

xs:dayTimeDuration("P3DT08H34M12.143S") =
  xs:untypedAtomic("P3DT08H34M12.143S")