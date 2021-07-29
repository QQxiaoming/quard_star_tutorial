(:*******************************************************:)
(: Test: K2-SeqExprTreat-1                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: An xs:decimal, despite being a valid integer, cannot be treated as an xs:integer. :)
(:*******************************************************:)
xs:decimal(3) treat as xs:integer