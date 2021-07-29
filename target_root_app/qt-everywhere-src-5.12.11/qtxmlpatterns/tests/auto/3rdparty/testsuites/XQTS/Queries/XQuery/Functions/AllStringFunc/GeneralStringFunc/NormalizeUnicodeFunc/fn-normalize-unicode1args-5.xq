(:*******************************************************:)
(:Test:fn-normalize-unicode1args-5:)
(:Written By:Joanne Tong:)
(:Date:2005-09-29T15:55:46+01:00:)
(:Purpose:Test fn:normalize-unicode on combining characters for LATIN CAPITAL LETTER A WITH RING (w/ ACUTE) and ANGSTROM SIGN:)
(:*******************************************************:)

matches('&#x0041;&#x030A;', normalize-unicode('&#x0041;&#x030A;'))