(:*******************************************************:)
(: Test: K2-DirectConElemAttr-45                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: xml:base attributes do not affect the static base-uri(#2). :)
(:*******************************************************:)
declare base-uri "http://example.com/level/file.ext";
                    <e xml:base="http://example.com/2/2">{ static-base-uri()}</e>