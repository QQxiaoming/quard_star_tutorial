(:*******************************************************:)
(:Test: timezone-from-date-3                             :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "timezone-from-date" function   :)
(:involving a "numeric-less-than" operation (lt operator):)
(:*******************************************************:)

fn:timezone-from-date(xs:date("1999-12-31+05:00")) lt fn:timezone-from-date(xs:date("1999-12-31+06:00"))