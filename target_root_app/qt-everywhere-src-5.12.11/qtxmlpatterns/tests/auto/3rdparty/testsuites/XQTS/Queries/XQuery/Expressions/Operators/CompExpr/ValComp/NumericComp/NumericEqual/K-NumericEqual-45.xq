(:*******************************************************:)
(: Test: K-NumericEqual-45                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Test automatic xs:untypedAtomic conversion.  :)
(:*******************************************************:)
count(xs:untypedAtomic("1") to 3) eq 3