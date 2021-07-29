(:*******************************************************:)
(: Test: K-SeqExprCast-902                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Cast a simple xs:untypedAtomic value to "2004-10-13" . :)
(:*******************************************************:)
xs:date(xs:untypedAtomic(
      "2004-10-13"
    )) eq xs:date("2004-10-13")