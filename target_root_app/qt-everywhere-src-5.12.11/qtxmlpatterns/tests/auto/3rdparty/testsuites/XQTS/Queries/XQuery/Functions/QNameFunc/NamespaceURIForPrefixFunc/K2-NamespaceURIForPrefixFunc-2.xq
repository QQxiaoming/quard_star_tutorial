(:*******************************************************:)
(: Test: K2-NamespaceURIForPrefixFunc-2                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Test that the correct default namespace is picked up(#2). :)
(:*******************************************************:)
namespace-uri-for-prefix("", exactly-one(<e xmlns="http://www.example.com/A" xmlns:A="http://www.example.com/C">
              <b xmlns:B="http://www.example.com/C" xmlns=""/>
      </e>/b)) eq ""