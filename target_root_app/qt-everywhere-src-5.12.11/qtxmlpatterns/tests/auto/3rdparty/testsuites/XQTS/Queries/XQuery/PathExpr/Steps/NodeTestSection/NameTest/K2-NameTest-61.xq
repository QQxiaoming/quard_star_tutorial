(:*******************************************************:)
(: Test: K2-NameTest-61                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Apply processing-instruction() to the empty sequence. :)
(:*******************************************************:)
<e>{for $PI in ()/processing-instruction() return ()}</e>