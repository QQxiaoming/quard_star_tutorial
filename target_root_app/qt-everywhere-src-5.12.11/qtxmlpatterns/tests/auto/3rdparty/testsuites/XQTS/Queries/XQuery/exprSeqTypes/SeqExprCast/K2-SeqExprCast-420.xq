(:*******************************************************:)
(: Test: K2-SeqExprCast-420                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The whitespace facet for xs:anyURI is collapse. :)
(:*******************************************************:)
xs:anyURI("           
        http://example.com/  
        foo.xml         
              
            ")