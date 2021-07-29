(:*******************************************************:)
(: Test: K2-DirectConElemNamespace-25                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Test that an empty namespace declaration is handled correctly. :)
(:*******************************************************:)
declare namespace b = "http://www.example.com/";
                    empty(<e xmlns="http://www.example.com/"><d xmlns=""><b/></d></e>/b:d/b:b)