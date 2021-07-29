(:*******************************************************:)
(: Test: K2-SeqExprInstanceOf-69                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Check that xs:language is not a child of xs:NMTOKEN. :)
(:*******************************************************:)
xs:language("ncname") instance of xs:NMTOKEN