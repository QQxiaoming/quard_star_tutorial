(:*******************************************************:)
(: Test: K-SeqExprCast-1371                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:22+01:00                       :)
(: Purpose: Applying fn:boolean() to a value of type xs:anyURI that has the lexical value ' "http://www.example.com/an/arbitrary/URI.ext" ' should result in the boolean value true. :)
(:*******************************************************:)

          boolean(xs:anyURI("http://www.example.com/an/arbitrary/URI.ext"))