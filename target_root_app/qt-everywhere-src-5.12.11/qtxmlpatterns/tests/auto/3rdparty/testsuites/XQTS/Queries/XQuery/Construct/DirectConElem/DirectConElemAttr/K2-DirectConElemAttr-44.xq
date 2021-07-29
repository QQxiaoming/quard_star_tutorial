(:*******************************************************:)
(: Test: K2-DirectConElemAttr-44                         :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: xml:base attributes do not affect the static base-uri. :)
(:*******************************************************:)

                declare base-uri "http://example.com/level/file.ext";
                    <e xml:base="../">{ static-base-uri()}</e>