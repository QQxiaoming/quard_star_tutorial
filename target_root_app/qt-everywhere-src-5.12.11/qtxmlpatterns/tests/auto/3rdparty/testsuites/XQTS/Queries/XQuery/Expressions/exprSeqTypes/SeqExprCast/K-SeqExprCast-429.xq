(:*******************************************************:)
(: Test: K-SeqExprCast-429                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: An empty string is a valid lexical representation of xs:string. :)
(:*******************************************************:)
xs:string("")
            eq
            xs:string("")
          