(:*******************************************************:)
(:Test:fn-normalize-unicode2args-4:)
(:Written By:Joanne Tong:)
(:Date:2005-09-29T15:58:10+01:00:)
(:Purpose:Test fn:normalize-unicode where the second argument is the zero-length string, no normalization is performed:)
(:*******************************************************:)

(normalize-unicode('&#x00C5;', '') eq normalize-unicode('&#x212B;', ''))