(:*******************************************************:)
(: Test: K2-DirectConOther-48                            :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure xml:id is properly normalized, when using a direct constructor. The value is also an invalid xs:ID value, hence the allowed error code. :)
(:*******************************************************:)
string(exactly-one(<e xml:id="    ab    c

           
                    d            "/>/@*))