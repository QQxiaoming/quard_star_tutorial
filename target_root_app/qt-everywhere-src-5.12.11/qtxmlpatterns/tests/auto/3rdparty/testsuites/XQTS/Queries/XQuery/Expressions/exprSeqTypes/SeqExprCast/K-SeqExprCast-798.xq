(:*******************************************************:)
(: Test: K-SeqExprCast-798                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Cast a simple xs:untypedAtomic value to "2002-10-10T12:00:00-05:00" . :)
(:*******************************************************:)
xs:dateTime(xs:untypedAtomic(
      "2002-10-10T12:00:00-05:00"
    )) eq xs:dateTime("2002-10-10T12:00:00-05:00")