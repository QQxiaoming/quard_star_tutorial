(:*******************************************************:)
(: Test: K-SeqExprCast-590                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Cast a simple xs:untypedAtomic value to "6789" . :)
(:*******************************************************:)
xs:integer(xs:untypedAtomic(
      "6789"
    )) eq xs:integer("6789")