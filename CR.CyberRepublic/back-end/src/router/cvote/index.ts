import Base from '../Base'

import create_draft from './create_draft'
import propose_suggestion from './propose_suggestion'
import update_draft from './update_draft'
import delete_draft from './delete_draft'
import create from './create'
import list from './list'
import list_public from './list_public'
import get from './get'
import update from './update'
import vote from './vote'
import finish from './finish'
import unfinish from './unfinish'
import update_notes from './update_notes'
import listcrcandidates from './listcrcandidates'
import onchain from './onchain'
import all_search from './all_search'
import get_proposal from './get_proposal'
import callback from './callback'
import check_signature from './check_signature'
import memberVote from './memberVote'
import pollProposalState from './poll_proposal_state'
import pollVoteState from './poll_vote_state'
import pollMemberVote from './poll_member_vote'

export default Base.setRouter([
  {
    path: '/create_draft',
    router: create_draft,
    method: 'post'
  },
  {
    path: '/poll_proposal_state',
    router: pollProposalState,
    method: 'post'
  },
  {
    path: '/propose_suggestion',
    router: propose_suggestion,
    method: 'post'
  },
  {
    path: '/update_draft',
    router: update_draft,
    method: 'post'
  },
  {
    path: '/delete_draft',
    router: delete_draft,
    method: 'post'
  },
  {
    path: '/create',
    router: create,
    method: 'post'
  },
  {
    path: '/list_public',
    router: list_public,
    method: 'get'
  },
  {
    path: '/list',
    router: list,
    method: 'get'
  },
  {
    path: '/get/:id',
    router: get,
    method: 'get'
  },
  {
    path: '/update',
    router: update,
    method: 'post'
  },
  {
    path: '/vote',
    router: vote,
    method: 'post'
  },
  {
    path: '/finish',
    router: finish,
    method: 'post'
  },
  {
    path: '/unfinish',
    router: unfinish,
    method: 'post'
  },
  {
    path: '/update_notes',
    router: update_notes,
    method: 'post'
  },
  {
    path: '/listcrcandidates',
    router: listcrcandidates,
    method: 'post'
  },
  {
    path: '/reviewproposal-url',
    router: onchain,
    method: 'post'
  },
  {
    path: '/signature',
    router: check_signature,
    method: 'post'
  },
  {
    path: '/member_vote',
    router: memberVote,
    method: 'post'
  },
  {
    path: '/callback',
    router: callback,
    method: 'get'
  },
  {
    path: '/all_search',
    router: all_search,
    method: 'get'
  },
  {
    path: '/get_proposal/:id',
    router: get_proposal,
    method: 'get'
  },
  {
    path: '/poll_vote_state',
    router: pollVoteState,
    method: 'post'
  },
  {
    path: '/poll_member_vote',
    router: pollMemberVote,
    method: 'post'
  }
])
