(:*******************************************************:)
(: Test: K2-DefaultNamespaceProlog-13                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Variables cannot occur before namespace declarations. :)
(:*******************************************************:)
declare variable $variable := 1;
declare default element namespace "http://example.com";
1