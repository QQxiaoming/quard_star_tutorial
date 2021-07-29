(:*******************************************************:)
(: Test: K-SeqRemoveFunc-17                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:23+01:00                       :)
(: Purpose: Combine fn:remove() with operator 'eq'.      :)
(:*******************************************************:)
remove((4, xs:untypedAtomic("4")), 1) eq 4