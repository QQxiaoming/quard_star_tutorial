(:*******************************************************:)
(: Test: K2-DefaultNamespaceProlog-1                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: In direct element constructors, the declared default element namespace is respected. :)
(:*******************************************************:)
declare default element namespace "http://example.com/defelementns";
 namespace-uri(<foo/>)