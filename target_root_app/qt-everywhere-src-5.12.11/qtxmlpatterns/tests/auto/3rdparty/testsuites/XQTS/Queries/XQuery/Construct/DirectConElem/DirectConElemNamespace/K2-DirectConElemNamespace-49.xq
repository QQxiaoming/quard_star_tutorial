(:*******************************************************:)
(: Test: K2-DirectConElemNamespace-49                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Use a 'declare default element namespace' in addition to binding to the empty prefix. :)
(:*******************************************************:)
declare default element namespace "http://www.example.com/A";
<anElement xmlns="http://www.example.com/B"/>