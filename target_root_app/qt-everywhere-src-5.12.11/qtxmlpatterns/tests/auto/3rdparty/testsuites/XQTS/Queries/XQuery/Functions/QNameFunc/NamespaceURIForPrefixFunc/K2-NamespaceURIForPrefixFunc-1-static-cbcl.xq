(:*******************************************************:)
(: Test: K2-NamespaceURIForPrefixFunc-1                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-10-03T14:53:33+01:00                       :)
(: Purpose: Test that the correct default namespace is picked up. :)
(:*******************************************************:)
declare namespace p = "http://www.example.com/B";
          namespace-uri-for-prefix("", exactly-one(<e xmlns="http://www.example.com/A" xmlns:A="http://www.example.com/C">
              <b xmlns:B="http://www.example.com/C" xmlns="http://www.example.com/B"/>
      </e>/p:b))
