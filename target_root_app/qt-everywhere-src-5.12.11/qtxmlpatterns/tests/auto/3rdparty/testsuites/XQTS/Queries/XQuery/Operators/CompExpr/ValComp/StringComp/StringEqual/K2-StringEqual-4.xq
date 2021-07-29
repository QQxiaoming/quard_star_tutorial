(:*******************************************************:)
(: Test: K2-StringEqual-4                                :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Compare two non-matching values returned from fn:upper-case(). :)
(:*******************************************************:)
declare variable $vA as xs:string := ("B STRING", current-time(), string(<e>content</e>))[1] treat as xs:string;
    declare variable $vB as xs:string := ("no match", current-time(), string(<e>content</e>))[1] treat as xs:string;
    (upper-case($vA) eq upper-case($vB))