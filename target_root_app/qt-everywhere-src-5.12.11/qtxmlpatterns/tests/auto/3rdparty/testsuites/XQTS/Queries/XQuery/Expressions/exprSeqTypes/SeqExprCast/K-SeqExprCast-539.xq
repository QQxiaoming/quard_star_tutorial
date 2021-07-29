(:*******************************************************:)
(: Test: K-SeqExprCast-539                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Applying fn:boolean() to a value of type xs:decimal that has the lexical value ' "10.01" ' should result in the boolean value true. :)
(:*******************************************************:)

          boolean(xs:decimal("10.01"))