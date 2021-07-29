(:*******************************************************:)
(:Test: year-from-date-3                                 :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "year-from-date" function       :)
(:involving a "numeric-less-than" operation (le operator):)
(:*******************************************************:)

fn:year-from-date(xs:date("1999-12-31Z")) le fn:year-from-date(xs:date("1999-12-31Z"))