(:*******************************************************:)
(:Test: timezone-from-date-4                             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 27, 2005                                    :)
(:Purpose: Evaluates The "timezone-from-date" function   :)
(:involving a "numeric-less-than" operation (le operator):)
(:*******************************************************:)

fn:timezone-from-date(xs:date("1999-12-31+01:00")) le fn:timezone-from-date(xs:date("1999-12-31+01:00"))