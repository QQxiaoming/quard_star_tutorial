(:*******************************************************:)
(: Test: K-SeqSUMFunc-22                                 :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: fn:sum() having an input sequence whose static type is xs:anyAtomicType. :)
(:*******************************************************:)
sum(remove((1.0, xs:float(1), 2, xs:untypedAtomic("3")), 1)) eq 6