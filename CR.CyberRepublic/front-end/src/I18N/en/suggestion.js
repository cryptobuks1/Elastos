import { CVOTE_TRACKING_STATUS, CVOTE_SUMMARY_STATUS } from '@/constant'

export default {
  title: {
    add: 'ADD A SUGGESTION',
    edit: 'EDIT SUGGESTION'
  },
  fields: {
    title: 'Title',
    preamble: 'Preamble',
    preambleSub: {
      proposal: 'CRC Proposal',
      title: 'Title',
      proposer: 'Proposer',
      status: 'Status',
      created: 'Created'
    },
    abstract: 'Abstract',
    goal: 'Goal',
    motivation: 'Motivation',
    relevance: 'Relevance',
    budget: 'Budget',
    type: 'Type',
    plan: 'Implementation Plan',
    vote: 'Vote',
    tracking: 'Tracking',
    summary: 'Summary'
  },
  btn: {
    tracking: {
      reject: 'Reject & Comment',
      approve: 'Approve & Publish'
    },
    summary: {
      reject: 'Reject & Comment',
      approve: 'Approve & Publish'
    },
    makeIntoProposal: 'Make Into Proposal'
  },
  text: {
    tracking: {
      reviewDetails: 'Review Details',
      notice: {
        header: '',
        footer: ''
      }
    },
    summary: {
      reviewDetails: 'Review Details',
      notice: {
        header: '',
        footer: ''
      }
    }
  },
  status: {
    tracking: {
      [CVOTE_TRACKING_STATUS.DRAFT]: 'DRAFT',
      [CVOTE_TRACKING_STATUS.REVIEWING]: 'BEING REVIEWED',
      [CVOTE_TRACKING_STATUS.PUBLISHED]: 'APPROVED ✓',
      [CVOTE_TRACKING_STATUS.REJECT]: 'REJECTED !'
    },
    summary: {
      [CVOTE_SUMMARY_STATUS.DRAFT]: 'DRAFT',
      [CVOTE_SUMMARY_STATUS.REVIEWING]: 'BEING REVIEWED',
      [CVOTE_SUMMARY_STATUS.PUBLISHED]: 'APPROVED ✓',
      [CVOTE_SUMMARY_STATUS.REJECT]: 'REJECTED !'
    },
    trackingRaw: {
      undefined: '',
      [CVOTE_TRACKING_STATUS.DRAFT]: 'DRAFT',
      [CVOTE_TRACKING_STATUS.REVIEWING]: 'BEING REVIEWED',
      [CVOTE_TRACKING_STATUS.PUBLISHED]: 'APPROVED',
      [CVOTE_TRACKING_STATUS.REJECT]: 'REJECTED'
    },
    summaryRaw: {
      undefined: '',
      [CVOTE_SUMMARY_STATUS.DRAFT]: 'DRAFT',
      [CVOTE_SUMMARY_STATUS.REVIEWING]: 'BEING REVIEWED',
      [CVOTE_SUMMARY_STATUS.PUBLISHED]: 'APPROVED',
      [CVOTE_SUMMARY_STATUS.REJECT]: 'REJECTED'
    }
  },
  form: {
    button: {
      continue: 'Continue',
      cancel: 'Cancel',
      saveDraft: 'Save as Draft',
      save: 'Save'
    },
    tracking: {
      btn: {
        submit: 'Submit for Review'
      },
      add: 'Add Tracking Plan'
    },
    summary: {
      btn: {
        submit: 'Submit for Review'
      },
      add: 'Add Summary'
    },
    fields: {
      title: 'Title',
    },
    type: {
      newMotion: 'New Motion',
      motionAgainst: 'Motion Against',
      anythingElse: 'Anything Else'
    },
    note: {
      type: 'Select a suggestion type.',
      abstract:
        'Provide a brief description of the proposal content (no more than 200 words).',
      goal:
        'Describe the desired results achieved by implementing the proposal. Goals should be clear and measurable.',
      motivation:
        'Describe why this proposal is proposed. Motivation is critical for proposals that try to improve Elastos or CR. It should be clearly explained here why the existing mechanism is not enough to solve the problems the proposal wants to solve.',
      motivationHighlight:
        'Proposals with insufficient motivation are highly likely to be rejected.',
      relevance:
        'If the proposal is related to other CRC proposals, the proposal number of each related proposal should be provided here. The relationship between the proposal and each related proposal should be described. If there are conflicts with Elastos technology or other CRC proposals, the proposal must describe these conflicts and explain how to deal with them.',
      budget:
        'If the implementation of the proposal requires financial support from the CRC, describe the overall budget and expenditure plan. This financial plan should be aligned with the implementation plan.',
      type: 'Select a proposal type.',
      plan:
        'Describe what methods and processes will be used to achieve goals, and a brief introduction of the executing person or team should be listed here as well. If proposal has a long implementation timeline, it should set some checkpoints in the implementation process. The interval between two checkpoints should be no more than 3 months. The checkpoints should be clear and measurable as the proposed goals.',
      tracking:
        'This part is updated by the proposer according to the progress of the proposal, including the achievement of goal and budget usage. It is used to present the implementation status of proposal according to the checkpoints in the implementation plan or the goals of the proposal. The CRC Secretariat is responsible to review and verify this part.',
      summary:
        'When proposal is completed, its proposer should submit a summary of the proposal implementation, including the achievement of goals and financial report. The CRC Secretariat is responsible for the review of this part.'
    },
    error: {
      required: 'This field is required',
      tooLong: 'This field is too long',
      limit200: 'You can only type 200 words max.'
    }
  },
  msg: {
    rejected: 'Rejected successfully',
    approved: 'Approved and published successfully',
    draftSaved:
      'Saved as draft successfully, you can check it in proposal list page',
    proposalPublished: 'Proposal published successfully'
  },
  modal: {
    addTagComment: 'Add Comment',
    confirm: 'Confirm',
    cancel: 'Cancel',
    pleaseUpdate:
      'Please update the Suggestion with the requested info and notify the council/community through a comment.',
    commentsFromCouncil: 'Comments from Council/Admin:'
  },
  tag: {
    show: 'Show',
    type: {
      UNDER_CONSIDERATION: 'Under Council Consideration',
      INFO_NEEDED: 'More Info Required',
      ADDED_TO_PROPOSAL: 'Added to Proposal'
    }
  }
}
