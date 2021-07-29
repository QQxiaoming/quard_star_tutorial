(:*******************************************************:)
(:Test:fn-normalize-unicode1args-6:)
(:Written By:Joanne Tong:)
(:Date:2005-09-29T15:56:41+01:00:)
(:Purpose:Test fn:normalize-unicode on combining characters for LATIN CAPITAL LETTER A WITH RING (w/ ACUTE) and ANGSTROM SIGN:)
(:*******************************************************:)

(normalize-unicode('&#x00C5;') eq normalize-unicode('&#x212B;'))