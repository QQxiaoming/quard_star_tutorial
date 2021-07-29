(:*******************************************************:)
(: Test: K2-DefaultNamespaceProlog-16                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Options cannot occur before namespace declarations. :)
(:*******************************************************:)
declare option local:myOption "foo";
declare default element namespace "http://example.com";
1