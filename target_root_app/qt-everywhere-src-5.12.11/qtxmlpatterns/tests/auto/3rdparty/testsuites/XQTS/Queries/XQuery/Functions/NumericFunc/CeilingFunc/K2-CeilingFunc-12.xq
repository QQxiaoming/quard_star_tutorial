(:*******************************************************:)
(: Test: K2-CeilingFunc-12                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure the return type is properly inferred for byte. :)
(:*******************************************************:)
ceiling(xs:byte(<e>0</e>)) instance of xs:byte