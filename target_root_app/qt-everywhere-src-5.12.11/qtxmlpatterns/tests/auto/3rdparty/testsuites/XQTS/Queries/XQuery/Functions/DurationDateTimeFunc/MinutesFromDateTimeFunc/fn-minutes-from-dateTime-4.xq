(:*******************************************************:)
(:Test: minutes-from-dateTime-3                          :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "minutes-from-dateTime" function:)
(:involving a "numeric-less-than" operation (le operator):)
(:*******************************************************:)

fn:minutes-from-dateTime(xs:dateTime("1999-12-31T21:20:00-05:00")) le fn:minutes-from-dateTime(xs:dateTime("1999-12-31T21:20:00-05:00"))