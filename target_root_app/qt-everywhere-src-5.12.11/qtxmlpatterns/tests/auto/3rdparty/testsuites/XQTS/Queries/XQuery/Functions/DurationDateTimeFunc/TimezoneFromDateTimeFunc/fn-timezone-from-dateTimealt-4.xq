(:*******************************************************:)
(:Test: fn-timezone-from-dateTime-4                      :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 27, 2005                                    :)
(:Purpose: Evaluates The "timezone-from-dateTime" function  :)
(: uses as part of a numeric-less-than expression (le operator).:)
(:*******************************************************:)

fn:timezone-from-dateTime(xs:dateTime("1999-12-31T12:00:00+02:00")) le fn:timezone-from-dateTime(xs:dateTime("1999-12-30T12:00:00+03:00"))