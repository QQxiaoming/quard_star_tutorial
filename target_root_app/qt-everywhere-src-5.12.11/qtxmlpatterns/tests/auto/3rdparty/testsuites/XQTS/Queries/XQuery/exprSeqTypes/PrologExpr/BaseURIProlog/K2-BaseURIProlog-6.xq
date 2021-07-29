(:*******************************************************:)
(: Test: K2-BaseURIProlog-6                              :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The static base-uri is not affected by xml:base declarations on direct element constructors. :)
(:*******************************************************:)
declare base-uri "http://example.com/BASEURI";
<e xml:base="../">
    {fn:static-base-uri()}
</e>