(:*******************************************************:)
(: Test: K2-NamespaceURIForPrefixFunc-3                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-10-03T14:53:33+01:00                       :)
(: Purpose: Test that the correct default namespace is picked up(#3). :)
(:*******************************************************:)
declare namespace p = "http://www.example.com/A";
              namespace-uri-for-prefix("", exactly-one(<e xmlns="http://www.example.com/A" xmlns:A="http://www.example.com/C">
              <b xmlns:B="http://www.example.com/C" />
      </e>/p:b))
