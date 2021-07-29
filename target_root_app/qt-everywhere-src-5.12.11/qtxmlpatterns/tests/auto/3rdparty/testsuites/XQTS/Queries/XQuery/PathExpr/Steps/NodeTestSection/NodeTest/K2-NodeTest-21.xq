(:*******************************************************:)
(: Test: K2-NodeTest-21                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure 'element(local:ncname)' is parsed correctly when inside document-node(). :)
(:*******************************************************:)
declare namespace ex = "http://www.example.com/";
              declare function local:userFunction()
              {
                document-node(element(local:ncname))
              };
              1