(:*******************************************************:)
(: Test: K-SeqExprCast-1318                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Cast a simple xs:untypedAtomic value to "0FB7" . :)
(:*******************************************************:)
xs:hexBinary(xs:untypedAtomic(
      "0FB7"
    )) eq xs:hexBinary("0FB7")