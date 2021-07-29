(:*******************************************************:)
(: Test: K2-DefaultNamespaceProlog-2                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: 'xmlns' declarations on direct element constructors override the declared default element namespace. :)
(:*******************************************************:)
declare default element namespace "http://example.com/defelementns";
    namespace-uri(<foo xmlns="http://example.com/overriden"/>)