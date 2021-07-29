(:*******************************************************:)
(: Test: K-SeqExprCast-1369                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: An empty string is a valid lexical representation of xs:anyURI. :)
(:*******************************************************:)
xs:anyURI("")
            eq
            xs:anyURI("")
          