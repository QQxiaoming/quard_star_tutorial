(:*******************************************************:)
(: Test: K-SeqExprCast-591                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Applying fn:boolean() to a value of type xs:integer that has the lexical value ' "6789" ' should result in the boolean value true. :)
(:*******************************************************:)

          boolean(xs:integer("6789"))