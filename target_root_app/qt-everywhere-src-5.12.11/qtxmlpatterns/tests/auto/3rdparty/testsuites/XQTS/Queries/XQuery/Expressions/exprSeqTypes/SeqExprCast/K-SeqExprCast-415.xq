(:*******************************************************:)
(: Test: K-SeqExprCast-415                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: An empty string is a valid lexical representation of xs:untypedAtomic. :)
(:*******************************************************:)
xs:untypedAtomic("")
            eq
            xs:untypedAtomic("")
          