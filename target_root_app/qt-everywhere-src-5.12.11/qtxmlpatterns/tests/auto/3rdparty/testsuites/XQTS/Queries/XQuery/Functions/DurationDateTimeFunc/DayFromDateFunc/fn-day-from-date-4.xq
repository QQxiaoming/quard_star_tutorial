(:*******************************************************:)
(:Test: day-from-date-4                                  :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "day-from-date" function        :)
(:involving a "numeric-less-than" operation (le operator):)
(:*******************************************************:)

fn:day-from-date(xs:date("1999-12-31Z")) le fn:day-from-date(xs:date("1999-12-31Z"))